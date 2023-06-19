import reactDom from 'react-dom'

const ModalPortal = (children) => {
    const visibleArea = document.getElementById("modal");
    return (
        reactDom.createPortal(children, visibleArea)//children : 포탈을 사용해 계층 밖으로 보낼 대상 컴포넌트, visibleArea ; 포탈로 이동할 목적지지
    );
};

export default ModalPortal;